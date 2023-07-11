create database if not exists origin;

use origin;

create table if not exists plants(
    plant_id int not null AUTO_INCREMENT,
    name varchar(40) not null,
    preferred_watering_time datetime null,
    primary key (plant_id)
);

create table if not exists devices(
    device_id varchar(50) not null,
    plant_id int not null,
    primary key (device_id),
    constraint foreign key fk_plant_id (plant_id) references plants (plant_id)
);

insert into plants (name, preferred_watering_time)
select * from (select 'lechuga', '2023-01-01 05:00:00') as tmp
where not exists (
    select name from plants where name = 'lechuga'
) limit 1;

insert into devices (device_id, plant_id)
select * from (select '000', 1) as tmp
where not exists (
    select device_id from devices where device_id = '000'
) limit 1;
